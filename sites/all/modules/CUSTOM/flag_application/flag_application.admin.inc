<?php

/**
 * @param $form
 * @param $form_state
 * @return mixed
 * Flag Application Administration
 * @throws Exception
 */
function flag_application_form($form, $form_state) {
    $form = [];

    $header = [
        'title' => t('Flagged Content'),
        'name' => t('Username'),
        'entity_id' => t('Node ID'),
        'uid' => t('User ID'),
        'flagging_id' => t('Flag ID'),
        'status' => t('Status')
    ];

    $sql = db_select('flag_application', 'fa')->extend('PagerDefault');
    $sql->join('flagging', 'f', 'f.flagging_id = fa.flagging_id');
    $sql->join('node', 'n', 'f.entity_id = n.nid');
    $sql->join('users', 'u', 'f.uid = u.uid');
    $sql
        ->fields('fa', ['status'])
        ->fields('f', ['entity_id', 'timestamp', 'flagging_id'])
        ->fields('n', ['title'])
        ->fields('u', ['name', 'uid'])
        ->orderBy('timestamp', 'DESC')
        ->limit(25);

    $results = $sql->execute();

    $rows = [];

    foreach ($results as $result) {
        switch ($result->status) {
            case 0:
                $status = 'Pending';
                break;
            case 1:
                $status = 'Approved';
                break;
            case 2:
                $status = 'Denied';
                break;
        }

        $rows[] = [
            'title' => l($result->title, 'node/'. $result->entity_id),
            'name' => l($result->name, 'user/'. $result->uid),
            'entity_id' => $result->entity_id,
            'uid' => $result->uid,
            'flagging_id' => $result->flagging_id,
            'status' => $status,
            '#attributes' => [
                'class' => ['application-row']
            ]
        ];
    }

    $form['flag_application_status ']['approvedeny'] = [
        '#type' => 'select',
        '#title' => 'Actions',
        '#options' => [
            1 => t('Approve'),
            2 => t('Deny')
        ]
    ];

    $form['flag_application_table'] = [
        '#type' => 'tableselect',
        '#header' => $header,
        '#options' => $rows,
        '#empty' => t('No Applications found.'),
        '#attributes' => [
            'class' => ['applications']
        ]
    ];

    $form['pager'] = [
        '#markup' => theme('pager')
    ];

    $form['submit'] = [
        '#type' => 'submit',
        '#value' => t('Submit')
    ];

    return $form;
}

function flag_application_form_validate($form, $form_state) {
    // dpm($form); dpm($form_state);

    $action = $form_state['values']['approvedeny'];

    if (!in_array($action, [1, 2])) {
        form_set_error('approvedeny', 'You have provided an incorrect value');
    }
}

function flag_application_form_submit($form, $form_state) {
    /*dpm($form);
    dpm($form_state); */

    $action = $form_state['values']['approvedeny'];

    $values = [];

    foreach ($form_state['values']['flag_application_table'] as $key => $value) {
        if (is_string($value) == TRUE) {
            $values[] = $form_state['complete form']['flag_application_table']['#options'][$value];
        }
    }

    flag_application_administration($action, $values);
}

function flag_application_administration($action, $values) {
    // dpm($action); dpm($values);

    $status = ($action == 1) ? 'Approved' : 'Denied';

    foreach ($values as $key => $value) {
        $num_updated = db_update('flag_application')
            ->fields([
                'status' => $action
            ])
            ->condition('flagging_id', $value['flagging_id'])
            ->execute();

        /*if (module_exists('rules')) {
            rules_invoke_event('flag_application_reviewed', $value['flagging_id'], $action);
        }*/

        watchdog('flag_application', t("!name's application for !node was !status"), [
            '!name' => $value['name'],
            '!node' => $value['title'],
            '!status' => $status
        ]);
    }
}