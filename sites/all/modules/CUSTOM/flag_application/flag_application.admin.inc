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